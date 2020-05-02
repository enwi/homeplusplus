import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { SubActionListComponent } from './sub-action-list.component';

describe('SubActionListComponent', () => {
  let component: SubActionListComponent;
  let fixture: ComponentFixture<SubActionListComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ SubActionListComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(SubActionListComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
