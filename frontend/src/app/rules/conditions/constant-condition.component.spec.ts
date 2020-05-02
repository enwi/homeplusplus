import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { ConstantConditionComponent } from './constant-condition.component';

describe('ConstantConditionComponent', () => {
  let component: ConstantConditionComponent;
  let fixture: ComponentFixture<ConstantConditionComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ ConstantConditionComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ConstantConditionComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
