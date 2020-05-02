import {TestBed} from '@angular/core/testing';

import {TasmotaParseService} from './tasmota-parse.service';

describe('TasmotaParseService', () => {
  beforeEach(() => TestBed.configureTestingModule({}));

  it('should be created', () => {
    const service: TasmotaParseService = TestBed.inject(TasmotaParseService);
    expect(service).toBeTruthy();
  });
});
