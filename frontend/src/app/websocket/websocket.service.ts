import {DOCUMENT} from '@angular/common';
import {Inject, Injectable, OnDestroy} from '@angular/core';
import {Router} from '@angular/router';
import {webSocket, WebSocketSubject, WebSocketSubjectConfig} from 'rxjs/webSocket';

import {AuthService} from '../auth/auth.service';

// either ws:// or wss://
const SERVER_PROTOCOL = 'ws://';
const SERVER_PORT = 9002;

class CustomWebsocketConfig implements WebSocketSubjectConfig<any> {
  public constructor(public url: string, private authService: AuthService) {}

  public serializer =
      (e: any) => {
        if (this.authService.isLoggedIn()) {
          const changed =
              Object.assign({idToken: this.authService.getIdToken()}, e);
          return JSON.stringify(changed);
        }
        return JSON.stringify(e);
      }

  public deserializer = (value: {data: string}) => {
    const json = value.data + '';
    return JSON.parse(json);
  }
}

@Injectable({providedIn: 'root'})
export class WebsocketService implements OnDestroy {
  private socket: WebSocketSubject<any>;

  constructor(
      @Inject(DOCUMENT) private document: Document,
      private authService: AuthService, private router: Router) {}

  public connect(): WebSocketSubject<any> {
    if (!this.socket || this.socket.closed) {
      const url =
          SERVER_PROTOCOL + this.document.location.hostname + ':' + SERVER_PORT;
      this.socket = this.create(url);
      console.log('websocket opened');
    }
    return this.socket;
  }

  private create(url: string): WebSocketSubject<any> {
    const socket =
        webSocket<any>(new CustomWebsocketConfig(url, this.authService));
    socket.subscribe({
      next: (value) => this.onMessage(value),
      complete: () => {
        // TODO: fix not executing subscribers when closed and reopened
        console.log('websocket closed');
      }
    });
    return socket;
  }


  public close(): void {
    if (this.socket) {
      this.socket.complete();
      this.socket.unsubscribe();
      this.socket = undefined;
    }
  }

  private onMessage(value: any): void {
    console.log('websocket message', value);
    if (value.hasOwnProperty('error')) {
      this.router.navigate(['/login']);
    }
  }

  ngOnDestroy(): void {
    this.close();
  }
}
