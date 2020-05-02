import {Injectable, OnDestroy} from '@angular/core';
import {BehaviorSubject, from, Observable, of, Subject, Subscription} from 'rxjs';
import {filter, map, take, takeUntil} from 'rxjs/operators';
import {WebSocketSubject} from 'rxjs/webSocket';

import {WebsocketChannelService} from '../websocket/websocket-channel.service';
import {WebsocketService} from '../websocket/websocket.service';

import {USER} from './mock-user';
import {User} from './user';

@Injectable({providedIn: 'root'})
export class UserService implements OnDestroy {
  private subject: Subject<any>;
  private $user: BehaviorSubject<User>;
  private user: User;
  private userValid = false;
  private userSubscription: Subscription;

  constructor(private channel: WebsocketChannelService) {
    this.user = USER;
    this.$user = new BehaviorSubject(this.user);
    this.initSubject();
  }

  /**
   * Initialise the subject for communication with the backend
   */
  private initSubject(): void {
    if (!this.subject) {
      this.subject = this.channel.getChannel('profile');
      this.subject.next({command: 'GET_USER'});
      this.userSubscription = this.subject.subscribe(message => this.handleUser(message));
    }
  }

  /**
   * Update the user
   * @param message The message received from backend
   */
  public handleUser(message: any): void {
    if (message !== undefined && message.userid !== undefined && message.user !== undefined &&
        message.pic !== undefined) {
      this.user.id = message.userid;
      this.user.name = message.user;
      this.user.picture = message.pic;
      this.userValid = true;
      this.$user.next(this.user);
      if (this.userSubscription) {
        this.userSubscription.unsubscribe();
      }
    }
  }

  /**
   * Update the user picture of this user
   * @param message The message received from backend
   */
  private HandlePicture(message: any): void {
    if (message) {
      if (message['pic'] && message['userid']) {
        if (message['userid'] === this.user.id) {
          this.user.picture = message['pic'];
          this.$user.next(this.user);
        }
      }
    }
  }

  /**
   * Check if the given password is correct for this user
   * @param password The password as string
   * @param message The message received from backend
   * @returns True if the given password is correct, false if not
   */
  private MessageVerifyPassword(password: string, message: any): boolean {
    if (message) {
      return message['pw'] === password && message['valid'] === true;
    }
    return undefined;
  }

  /**
   * Check if the set password was successfully updated
   * @param password Check if the set password was successfully updated
   * @param message The message received from backend
   * @returns True if password has been changed, false if not
   */
  private MessageUdpatePassword(password: string, message: any): boolean {
    if (message) {
      return message['pw'] === password && message['success'] === true;
    }
  }

  /**
   * Verify a given password for the given user
   * @param password The password to verify
   * @param userid The user identifier to verify the password for
   * @returns An observerable boolean which is true if the password is correct
   *     and false if not
   */
  public verifyPassword(password: string, userid: number): Observable<boolean> {
    this.initSubject();
    this.subject.next({command: 'CHECK_PW', pw: password, userid: userid});
    return this.subject.pipe(
        map((message: any) => this.MessageVerifyPassword(password, message)),
        filter(msg => msg !== undefined), take(1));
  }

  /**
   * Set/Change the passowrd of a given user
   * @param old_password The old password
   * @param new_password The new password
   * @param repeated_password The repeated new password
   * @param userid The user identifier
   * @returns An observerable boolean which is true if the password was changed
   *     and false if not
   */
  changePassword(
      old_password: string, new_password: string, repeated_password: string,
      userid: number): Observable<boolean> {
    this.initSubject();
    this.subject.next({
      command: 'CHANGE_PW',
      pw: {old: old_password, new: new_password, rep: repeated_password},
      userid: userid
    });
    return this.subject.pipe(
        map((message: any) =>
                this.MessageUdpatePassword(new_password, message)),
        filter(msg => msg !== undefined), take(1));
  }

  ngOnDestroy(): void {
    this.subject.unsubscribe();
    if (this.userSubscription) {
      this.userSubscription.unsubscribe();
    }
  }

  setProfileImg(file): void {
    this.user.picture = file;
    this.$user.next(this.user);

    this.initSubject();
    this.subject.next(
        {command: 'CHANGE_PICTURE', pic: file, userid: this.user.id});
  }

  /**
   * Change the username of this user
   * @param name The new username
   */
  setUsername(name): void {
    this.user.name = name;
    this.$user.next(this.user);
  }

  /**
   * Get an observerable of this user
   */
  getUser(): Observable<User> {
    return this.$user;
  }

  isUserValid(): boolean {
    return this.userValid;
  }
}
