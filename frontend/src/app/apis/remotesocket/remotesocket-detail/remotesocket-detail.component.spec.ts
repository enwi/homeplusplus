import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { RemoteSocketDetailComponent } from './remotesocket-detail.component';

describe('RemoteSocketDetailComponent', () => {
  let component: RemoteSocketDetailComponent;
  let fixture: ComponentFixture<RemotesocketDetailComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ RemoteSocketDetailComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(RemoteSocketDetailComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
