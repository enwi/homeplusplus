import { TestBed, inject } from '@angular/core/testing';

import { DeviceParseService } from './device-parse.service';

describe('DeviceParseService', () => {
  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [DeviceParseService]
    });
  });

  it('should be created', inject([DeviceParseService], (service: DeviceParseService) => {
    expect(service).toBeTruthy();
  }));
});
