import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { EditableConditionComponent } from './editable-condition.component';

describe('EditableConditionComponent', () => {
  let component: EditableConditionComponent;
  let fixture: ComponentFixture<EditableConditionComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ EditableConditionComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(EditableConditionComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
