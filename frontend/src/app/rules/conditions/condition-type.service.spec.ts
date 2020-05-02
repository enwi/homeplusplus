import { TestBed, inject } from '@angular/core/testing';

import { ConditionTypeService } from './condition-type.service';

describe('ConditionTypeService', () => {
  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [ConditionTypeService]
    });
  });

  it('should be created', inject([ConditionTypeService], (service: ConditionTypeService) => {
    expect(service).toBeTruthy();
  }));
});
