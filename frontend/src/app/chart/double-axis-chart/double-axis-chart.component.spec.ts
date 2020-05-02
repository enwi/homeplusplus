import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { DoubleAxisChartComponent } from './double-axis-chart.component';

describe('DoubleAxisChartComponent', () => {
  let component: DoubleAxisChartComponent;
  let fixture: ComponentFixture<DoubleAxisChartComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ DoubleAxisChartComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(DoubleAxisChartComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
