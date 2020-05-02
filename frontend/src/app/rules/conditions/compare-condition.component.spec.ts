import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { CompareConditionComponent } from './compare-condition.component';

describe('CompareConditionComponent', () => {
  let component: CompareConditionComponent;
  let fixture: ComponentFixture<CompareConditionComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ CompareConditionComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(CompareConditionComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
