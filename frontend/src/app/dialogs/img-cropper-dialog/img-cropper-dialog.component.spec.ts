import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { ImgCropperDialogComponent } from './img-cropper-dialog.component';

describe('ImgCropperDialogComponent', () => {
  let component: ImgCropperDialogComponent;
  let fixture: ComponentFixture<ImgCropperDialogComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ ImgCropperDialogComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ImgCropperDialogComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
