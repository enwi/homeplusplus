import { TestBed, inject } from '@angular/core/testing';

import { RemoteSocketParseService } from './remotesocket-parse.service';

describe('RemoteSocketParseService', () => {
  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [RemoteSocketParseService]
    });
  });

  it('should be created', inject([RemoteSocketParseService], (service: RemoteSocketParseService) => {
    expect(service).toBeTruthy();
  }));
});
