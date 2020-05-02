import { TestBed, inject } from '@angular/core/testing';

import { RemoteSocketService } from './remotesocket.service';

describe('RemotesocketService', () => {
  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [RemoteSocketService]
    });
  });

  it('should be created', inject([RemoteSocketService], (service: RemoteSocketService) => {
    expect(service).toBeTruthy();
  }));
});
