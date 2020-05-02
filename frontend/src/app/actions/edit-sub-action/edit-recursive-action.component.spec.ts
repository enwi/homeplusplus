import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { EditRecursiveActionComponent } from './edit-recursive-action.component';

describe('EditRecursiveActionComponent', () => {
  let component: EditRecursiveActionComponent;
  let fixture: ComponentFixture<EditRecursiveActionComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ EditRecursiveActionComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(EditRecursiveActionComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
