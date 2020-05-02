import { Injectable } from '@angular/core';
import { WebsocketChannelService } from '../websocket/websocket-channel.service';
import { Observable, Subject, Subscription } from 'rxjs';
import { Notification } from './notification';
import { map, filter } from 'rxjs/operators';

@Injectable({
  providedIn: 'root'
})
export class NotificationService {

  private subject: Subject<any>;

  constructor(private channel: WebsocketChannelService) { }

  private initSubject(): void {
    if (!this.subject) {
      this.subject = this.channel.getChannel('notifications');
    }
  }

  private messageToNotification(message: any): Notification | undefined {
    if (message && 'notification' in message) {
      return Object.assign(new Notification(), message['notification']);
    } else {
      return undefined;
    }
  }

  getNotifications(): Observable<Notification> {
    this.initSubject();
    return this.subject.pipe(map(message => this.messageToNotification(message)),
      filter(notification => notification != null));
  }
}
