import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { TasmotaDetailComponent } from './tasmota-detail.component';

describe('TasmotaDetailComponent', () => {
  let component: TasmotaDetailComponent;
  let fixture: ComponentFixture<TasmotaDetailComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ TasmotaDetailComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(TasmotaDetailComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
