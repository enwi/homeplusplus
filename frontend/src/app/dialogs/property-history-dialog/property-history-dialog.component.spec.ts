import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { PropertyHistoryDialogComponent } from './property-history-dialog.component';

describe('PropertyHistoryDialogComponent', () => {
  let component: PropertyHistoryDialogComponent;
  let fixture: ComponentFixture<PropertyHistoryDialogComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ PropertyHistoryDialogComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(PropertyHistoryDialogComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
