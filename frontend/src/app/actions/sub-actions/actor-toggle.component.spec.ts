import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { ActorToggleComponent } from './actor-toggle.component';

describe('ActorToggleComponent', () => {
  let component: ActorToggleComponent;
  let fixture: ComponentFixture<ActorToggleComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ ActorToggleComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ActorToggleComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
