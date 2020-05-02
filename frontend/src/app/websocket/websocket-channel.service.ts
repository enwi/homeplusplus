import { Injectable } from '@angular/core';
import { WebsocketService } from './websocket.service';
import { Subject, ConnectableObservable, Subscription, PartialObserver, Observable } from 'rxjs';
import { WebSocketSubject } from 'rxjs/webSocket';
import { AnonymousSubject } from 'rxjs/internal/Subject';
import { map, filter } from 'rxjs/operators';

export class ChannelSubject extends Subject<any> {
  private filteredSocket: Observable<any>;
  constructor(private socket: WebSocketSubject<any>, private name: string) {
    super();
    this.filteredSocket = socket.pipe(filter((value) => value && value['channel'] === name));
  }
  next(value: any): void {
    value['channel'] = this.name;
    this.socket.next(value);
  }
  error(error: any): void {
    this.socket.error(error);
  }
  complete(): void {
    this.next({ command: 'unsubscribeChannel' });
    this.socket.complete();
    super.complete();
  }
  unsubscribe(): void {
    this.next({ command: 'unsubscribeChannel' });
    this.socket.unsubscribe();
    super.unsubscribe();
  }
  subscribe(observerOrNext?: any, error?: (error: any) => void, complete?: () => void): Subscription {
    if (this.observers.length === 0) {
      this.next({ command: 'subscribeChannel' });
    }
    return super.subscribe(observerOrNext, error, complete)
      .add(this.filteredSocket.subscribe(observerOrNext, error, complete))
      .add(() => {
        if (this.observers.length === 0) {
          this.next({ command: 'unsubscribeChannel' });
        }
      });
  }
}

@Injectable({
  providedIn: 'root'
})
export class WebsocketChannelService {

  private socket: WebSocketSubject<any>;
  private subjects: Map<string, Subject<any>> = new Map<string, Subject<any>>();

  constructor(private websocketService: WebsocketService) { }

  public getChannel(name: string): Subject<any> {
    if (!this.socket) {
      this.socket = this.websocketService.connect();
    }
    if (!this.subjects.has(name)) {
      this.subjects.set(name, new ChannelSubject(this.socket, name));
    }
    return this.subjects.get(name);
  }
}
