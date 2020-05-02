import { TestBed, inject } from '@angular/core/testing';

import { SubActionTypeService } from './sub-action-type.service';

describe('SubActionTypeService', () => {
  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [SubActionTypeService]
    });
  });

  it('should be created', inject([SubActionTypeService], (service: SubActionTypeService) => {
    expect(service).toBeTruthy();
  }));
});
