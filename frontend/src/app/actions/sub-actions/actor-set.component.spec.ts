import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { ActorSetComponent } from './actor-set.component';

describe('ActorSetComponent', () => {
  let component: ActorSetComponent;
  let fixture: ComponentFixture<ActorSetComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ ActorSetComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ActorSetComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
