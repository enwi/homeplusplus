import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { AddRemotesocketComponent } from './add-remotesocket.component';

describe('AddRemotesocketComponent', () => {
  let component: AddRemotesocketComponent;
  let fixture: ComponentFixture<AddRemotesocketComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ AddRemotesocketComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(AddRemotesocketComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
