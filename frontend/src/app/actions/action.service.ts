import { Injectable, OnDestroy } from '@angular/core';
import { WebsocketChannelService } from '../websocket/websocket-channel.service';
import { Subject, Observable } from 'rxjs';
import { Action } from './action';
import { map, filter, scan } from 'rxjs/operators';

@Injectable({
  providedIn: 'root'
})
export class ActionService implements OnDestroy {

  constructor(private channel: WebsocketChannelService) { }

  private subject: Subject<any>;

  private messageToAction(message: any): Action | null | undefined {
    if (message && 'action' in message) {
      return Object.assign(new Action(), message.action);
    }
    return undefined;
  }

  private initSubject(): void {
    if (!this.subject || this.subject.closed) {
      this.subject = this.channel.getChannel('actions');
    }
  }

  getActions(includeHidden: boolean = false): Observable<Action> {
    this.initSubject();
    this.subject.next({ command: 'GET_ACTIONS' });
    return this.subject.pipe(map((message: any) => this.messageToAction(message)),
      filter(action => action != null && (action.visible || includeHidden)));
  }
  getAction(id: number, includeHidden: boolean = false): Observable<Action> {
    this.initSubject();
    this.subject.next({ command: 'GET_ACTION', id: id });
    return this.subject.pipe(map((message: any) => this.messageToAction(message)),
      filter(action => action != null && action.id === id && (action.visible || includeHidden)));
  }
  getActionsAsArray(includeHidden: boolean = false): Observable<Action[]> {
    // Transform to array, replace repeated actions
    return this.getActions(includeHidden).pipe(scan((list: Action[], action: Action) => {
      const index = list.findIndex(a => a.id === action.id);
      if (index !== -1) {
        list[index] = action;
      } else {
        list.push(action);
      }
      return list;
    }, []));
  }

  deleteAction(id: number): void {
    this.initSubject();
    this.subject.next({ command: 'DELETE_ACTION', id: id });
  }

  executeAction(id: number): void {
    this.initSubject();
    this.subject.next({ command: 'EXEC_ACTION', id: id });
  }

  addAction(action: Action): void {
    this.initSubject();
    this.subject.next({ command: 'ADD_ACTION', actionJSON: action });
  }

  ngOnDestroy(): void {
    this.subject.unsubscribe();
  }
}
