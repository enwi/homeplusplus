import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { EditActorToggleComponent } from './edit-actor-toggle.component';

describe('EditActorToggleComponent', () => {
  let component: EditActorToggleComponent;
  let fixture: ComponentFixture<EditActorToggleComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ EditActorToggleComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(EditActorToggleComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
