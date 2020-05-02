import { TestBed, inject } from '@angular/core/testing';

import { HueParseService } from './hue-parse.service';

describe('HueParseService', () => {
  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [HueParseService]
    });
  });

  it('should be created', inject([HueParseService], (service: HueParseService) => {
    expect(service).toBeTruthy();
  }));
});
