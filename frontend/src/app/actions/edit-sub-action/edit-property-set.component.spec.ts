import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { EditActorSetComponent } from './edit-actor-set.component';

describe('EditActorSetComponent', () => {
  let component: EditActorSetComponent;
  let fixture: ComponentFixture<EditActorSetComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ EditActorSetComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(EditActorSetComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
