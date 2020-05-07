import {AbstractControl, ValidationErrors} from '@angular/forms';
import {Observable, Observer} from 'rxjs';
import {map} from 'rxjs/operators';

import {UserService} from '../user/user.service';

export class PasswordValidator {
  static MatchPassword(AC: AbstractControl) {
    const password = AC.get('password').value;  // to get value in input tag
    const confirmPassword =
        AC.get('confirmPassword').value;  // to get value in input tag
    if (password !== confirmPassword) {
      AC.get('confirmPassword').setErrors({MatchPassword: true});
    } else {
      return null;
    }
  }

  static CreateVerifyPasswordValidator(userService: UserService) {
    return (control: AbstractControl): Observable<ValidationErrors|null> => {
      return userService.verifyPassword(control.value).pipe(map(res => {
        if (res) {
          return null;
        } else {
          return {Invalid: true};
        }
      }));
    };
  }
}
