import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { TimeConditionComponent } from './time-condition.component';

describe('TimeConditionComponent', () => {
  let component: TimeConditionComponent;
  let fixture: ComponentFixture<TimeConditionComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ TimeConditionComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(TimeConditionComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
