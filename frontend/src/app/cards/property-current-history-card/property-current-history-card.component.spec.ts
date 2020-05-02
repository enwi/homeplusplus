import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { PropertyCurrentHistoryCardComponent } from './property-current-history-card.component';

describe('PropertyCurrentHistoryCardComponent', () => {
  let component: PropertyCurrentHistoryCardComponent;
  let fixture: ComponentFixture<PropertyCurrentHistoryCardComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ PropertyCurrentHistoryCardComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(PropertyCurrentHistoryCardComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
