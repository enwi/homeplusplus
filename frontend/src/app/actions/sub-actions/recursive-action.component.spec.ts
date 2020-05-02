import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { RecursiveActionComponent } from './recursive-action.component';

describe('RecursiveActionComponent', () => {
  let component: RecursiveActionComponent;
  let fixture: ComponentFixture<RecursiveActionComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ RecursiveActionComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(RecursiveActionComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
