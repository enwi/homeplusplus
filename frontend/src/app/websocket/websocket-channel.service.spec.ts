import { TestBed, inject } from '@angular/core/testing';

import { WebsocketChannelService } from './websocket-channel.service';

describe('WebsocketChannelService', () => {
  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [WebsocketChannelService]
    });
  });

  it('should be created', inject([WebsocketChannelService], (service: WebsocketChannelService) => {
    expect(service).toBeTruthy();
  }));
});
