import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { CpuTemperatureCardComponent } from './cpu-temperature-card.component';

describe('CpuTemperatureCardComponent', () => {
  let component: CpuTemperatureCardComponent;
  let fixture: ComponentFixture<CpuTemperatureCardComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ CpuTemperatureCardComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(CpuTemperatureCardComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
