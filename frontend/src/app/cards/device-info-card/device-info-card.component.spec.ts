import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { DeviceInfoCardComponent } from './device-info-card.component';

describe('DeviceInfoCardComponent', () => {
  let component: DeviceInfoCardComponent;
  let fixture: ComponentFixture<DeviceInfoCardComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ DeviceInfoCardComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(DeviceInfoCardComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
