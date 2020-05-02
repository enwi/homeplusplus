import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { HueDetailComponent } from './hue-detail.component';

describe('HueDetailComponent', () => {
  let component: HueDetailComponent;
  let fixture: ComponentFixture<HueDetailComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ HueDetailComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(HueDetailComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
