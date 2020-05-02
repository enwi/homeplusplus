import {AfterViewInit, Component, EventEmitter, Inject, OnInit, ViewChild} from '@angular/core';
import {MAT_DIALOG_DATA, MatDialogRef} from '@angular/material/dialog';
// import {CropperSettings, ImageCropperComponent} from 'ng2-img-cropper';

@Component({
  selector: 'app-img-cropper-dialog',
  templateUrl: './img-cropper-dialog.component.html',
  styleUrls: ['./img-cropper-dialog.component.css'],
})
export class ImgCropperDialogComponent implements OnInit, AfterViewInit {
  // @Input() img: any;
  image: any = {};
  change = new EventEmitter<String>();
  // @ViewChild('cropper', {static: false}) cropper: ImageCropperComponent;
  // cropperSettings: CropperSettings = new CropperSettings();

  constructor(
      @Inject(MAT_DIALOG_DATA) public data: any,
      public dialogRef: MatDialogRef<ImgCropperDialogComponent>) {
    this.image = data.image;
  }

  ngOnInit() {
    // this.cropperSettings.noFileInput = true;
    // this.cropperSettings.width = 200;
    // this.cropperSettings.height = 200;
    // this.cropperSettings.minWidth = 200;
    // this.cropperSettings.minHeight = 200;
    // this.cropperSettings.croppedWidth = 800;
    // this.cropperSettings.croppedHeight = 800;
    // this.cropperSettings.preserveSize = true;
    // this.cropperSettings.showCenterMarker = false;
  }

  ngAfterViewInit(): void {
    // this.cropper.setImage(this.image);
  }

  cancel(): void {
    this.dialogRef.close();
  }

  submit(): void {
    this.dialogRef.close();
    this.change.emit(this.image.image);
  }
}
