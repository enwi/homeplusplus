import { Directive, HostListener } from '@angular/core';
import { WebsocketService } from './websocket.service';

@Directive({ selector: 'app-websocket-cleanup' })
export class WebsocketCleanup {
  constructor(private websocket: WebsocketService) { }

  @HostListener('window:unload')
  unloadHandler(): void {
    this.websocket.close();
  }
}
