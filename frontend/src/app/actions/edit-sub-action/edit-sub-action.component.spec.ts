import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { EditSubActionComponent } from './edit-sub-action.component';

describe('EditSubActionComponent', () => {
  let component: EditSubActionComponent;
  let fixture: ComponentFixture<EditSubActionComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ EditSubActionComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(EditSubActionComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
