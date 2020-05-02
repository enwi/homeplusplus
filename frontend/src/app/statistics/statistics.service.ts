import { Injectable, OnDestroy } from '@angular/core';
import { Observable, of, from, Subscription, Subject } from 'rxjs';
import { WebsocketService } from '../websocket/websocket.service';
import { map, filter } from 'rxjs/operators';
import { WebSocketSubject } from 'rxjs/webSocket';
import { WebsocketChannelService } from '../websocket/websocket-channel.service';

@Injectable({
  providedIn: 'root'
})
export class StatisticsService implements OnDestroy {

  constructor(private channel: WebsocketChannelService) { }

  private subject: Subject<any>;

  private messageToMemoryData(message: any): any {
    if (message) {
      return message['memory'];
    } else {
      return undefined;
    }
  }

  private messageToCPUUsageData(message: any): number {
    if (message) {
      return message['load'];
    } else {
      return undefined;
    }
  }

  private messageToCPUTemperatureData(message: any): any {
    if (message) {
      return message['temperature'];
    } else {
      return undefined;
    }
  }

  private initSubject(): void {
    if (!this.subject || this.subject.closed) {
      this.subject = this.channel.getChannel('stats');
    }
  }

  getMemoryData(): Observable<any> {
    this.initSubject();
    return this.subject.pipe(
      map((message: any) => this.messageToMemoryData(message)),
      filter(mem => mem !== undefined)
    );
  }

  getCPUUsage(): Observable<number> {
    this.initSubject();
    return this.subject.pipe(
      map((message: number) => this.messageToCPUUsageData(message)),
      filter(temp => temp !== undefined)
    );
  }

  getCPUTemperatureData(): Observable<any> {
    this.initSubject();
    return this.subject.pipe(
      map((message: any) => this.messageToCPUTemperatureData(message)),
      filter(temp => temp !== undefined)
    );
  }

  ngOnDestroy(): void {
    this.subject.unsubscribe();
  }
}
