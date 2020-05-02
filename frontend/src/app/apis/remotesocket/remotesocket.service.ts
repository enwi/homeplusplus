import { Injectable } from '@angular/core';
import { Subject } from 'rxjs';
import { WebsocketChannelService } from '../../websocket/websocket-channel.service';

@Injectable({
  providedIn: 'root'
})
export class RemoteSocketService {

  private subject: Subject<any>;

  constructor(private channel: WebsocketChannelService) { }

  private initChannel() {
    if (!this.subject) {
      this.subject = this.channel.getChannel('remoteSockets');
    }
  }

  public setHeader(id: number, name: string, location: string): void {
    this.initChannel();
    this.subject.next({ command: 'SET_HEADER', id: id, name: name, location: location });
  }

  public deleteRemoteSocket(id: number): void {
    this.initChannel();
    this.subject.next({ command: 'DELETE_REMOTE_SOCKET', id: id });
  }

  public setCode(id: number, code: string): void {
    this.initChannel();
    this.subject.next({ command: 'SET_CODE', id: id, code: code });
  }
}
