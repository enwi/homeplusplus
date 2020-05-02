import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { SensorConditionComponent } from './sensor-condition.component';

describe('SensorConditionComponent', () => {
  let component: SensorConditionComponent;
  let fixture: ComponentFixture<SensorConditionComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ SensorConditionComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(SensorConditionComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
