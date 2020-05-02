import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { CpuUsageCardComponent } from './cpu-usage-card.component';

describe('CpuUsageCardComponent', () => {
  let component: CpuUsageCardComponent;
  let fixture: ComponentFixture<CpuUsageCardComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ CpuUsageCardComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(CpuUsageCardComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
