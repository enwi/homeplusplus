import {Injectable, OnDestroy} from '@angular/core';
import {Subscription} from 'rxjs';
import {filter, map, take} from 'rxjs/operators';

import {AuthService} from '../auth/auth.service';
import {User} from '../user/user';
import {WebsocketService} from '../websocket/websocket.service';

import {UserService} from './../user/user.service';

@Injectable({providedIn: 'root'})
export class LoginService implements OnDestroy {
  userSubscription: Subscription;

  constructor(
      private websocketService: WebsocketService,
      private authService: AuthService, private userService: UserService) {}

  login(username: string, password: string) {
    // TODO: Implement SCRAM challenge-response authentication
    const socket = this.websocketService.connect();
    socket.next({login: {username: username, password: password}});
    if (!this.userSubscription) {
      this.userSubscription = socket.subscribe(message => this.userService.handleUser(message));
    }
    return socket.pipe(
        filter(value => value && ('idToken' in value || 'authFailed' in value)),
        map((value) => this.authService.login(value)), take(1));
  }

  requestUser() {
    const socket = this.websocketService.connect();
    socket.next({command: 'GET_USER'});
    if (!this.userSubscription) {
      this.userSubscription = socket.subscribe(message => this.userService.handleUser(message));
    }
  }

  ngOnDestroy() {
    if (this.userSubscription) {
      this.userSubscription.unsubscribe();
    }
  }
}
