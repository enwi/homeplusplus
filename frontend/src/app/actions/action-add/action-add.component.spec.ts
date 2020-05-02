import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { ActionAddComponent } from './action-add.component';

describe('ActionAddComponent', () => {
  let component: ActionAddComponent;
  let fixture: ComponentFixture<ActionAddComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ ActionAddComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ActionAddComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
