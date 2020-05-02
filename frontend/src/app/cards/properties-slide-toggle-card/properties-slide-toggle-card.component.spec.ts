import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { PropertiesSlideToggleCardComponent } from './properties-slide-toggle-card.component';

describe('PropertiesSlideToggleCardComponent', () => {
  let component: PropertiesSlideToggleCardComponent;
  let fixture: ComponentFixture<PropertiesSlideToggleCardComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ PropertiesSlideToggleCardComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(PropertiesSlideToggleCardComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
