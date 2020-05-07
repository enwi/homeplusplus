import {AfterViewInit, Component, OnInit, ViewChild} from '@angular/core';
import {AbstractControl, FormBuilder, FormGroup, ValidationErrors, Validators} from '@angular/forms';
import {MatDialog} from '@angular/material/dialog';
import {MatStepper} from '@angular/material/stepper';
import {Observable} from 'rxjs';
import {map} from 'rxjs/operators';

import {TextCardComponent} from '../cards/text-card/text-card.component';
import {ImgCropperDialogComponent} from '../dialogs/img-cropper-dialog/img-cropper-dialog.component';
import {LoadingDialogComponent} from '../dialogs/loading-dialog/loading-dialog.component';
import {PasswordValidator} from '../validators/password-validator';

import {User} from './user';
import {UserService} from './user.service';

@Component({
  selector: 'app-user',
  templateUrl: './user.component.html',
  styleUrls: ['../styles/card.css', './user.component.scss']
})
export class UserComponent implements OnInit, AfterViewInit {
  // profile img stuff
  @ViewChild('userImg', {static: false}) userImg: any;
  imageLoading = false;

  // password stuff
  firstFormGroup: FormGroup;
  secondFormGroup: FormGroup;
  hide1 = true;
  hide2 = true;
  password_valid = false;
  @ViewChild('stepper', {static: false}) stepper: MatStepper;

  // user stuff
  user$: Observable<User>;
  user: User;

  constructor(
      private userService: UserService, private _formBuilder: FormBuilder,
      public dialog: MatDialog) {}

  ngOnInit() {
    this.firstFormGroup = this._formBuilder.group({
      password: [
        '', {
          updateOn: 'submit',
          validators: Validators.required,
          asyncValidators:
              PasswordValidator.CreateVerifyPasswordValidator(this.userService)
        }
      ]
    });
    this.getUser();
    this.secondFormGroup = this._formBuilder.group(
        {
          password: ['', Validators.required],
          confirmPassword: ['', Validators.required]
        },
        {validator: PasswordValidator.MatchPassword});
    // make sure that user has to only press next button once due to async
    // validator
    this.firstFormGroup.statusChanges.subscribe(value => {
      if (this.firstFormGroup.valid && (this.stepper.selectedIndex === 0)) {
        this.stepper.selectedIndex = 1;
      }
    });
  }

  ngAfterViewInit(): void {
    // this.data = this.userImg.nativeElement;
  }

  getUser(): void {
    this.user$ = this.userService.getUser();
    this.user$.subscribe(user => this.user = user);
  }

  onFileChanged(event) {
    if (event.target.files && event.target.files.length > 0) {
      const reader = new FileReader();
      const file: File = event.target.files[0];
      const image: any = new Image();
      const spinnerDialog = this.dialog.open(LoadingDialogComponent);
      reader.onloadend = () => {
        image.src = reader.result;
        spinnerDialog.close();
        const dialogRef = this.dialog.open(
            ImgCropperDialogComponent, {data: {'image': image}});
        dialogRef.componentInstance.change.subscribe(e => this.onUploadImg(e));
        this.imageLoading = false;
      };

      this.imageLoading = true;
      reader.readAsDataURL(file);
    }
  }

  onUploadImg(event) {
    this.userService.setProfileImg(event);
  }

  onUsernameChange(event: string) {
    this.userService.setUsername(event);
  }

  changePassword() {
    const old_pw = this.firstFormGroup.get('password').value;
    const new_pw = this.secondFormGroup.get('password').value;
    const new_pw_conf = this.secondFormGroup.get('confirmPassword').value;

    this.userService.changePassword(old_pw, new_pw, new_pw_conf)
        .pipe(map(res => {
          console.log('res: ' + res ? 'true' : 'false');
          if (res) {
            this.secondFormGroup.setErrors({CchangedPassword: true});
            this.stepper.selectedIndex = 2;
          }
        }));
  }

  // image stuff
  test() {
    // let dialogRef = this.dialog.open(ImgCropperDialogComponent, { data:
    // this.data });
    const dialogRef = this.dialog.open(ImgCropperDialogComponent);
    const instance = dialogRef.componentInstance;
    // instance.img.src = this.data.src;
  }
}
