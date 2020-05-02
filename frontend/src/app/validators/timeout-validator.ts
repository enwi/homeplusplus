import { AbstractControl, ValidatorFn, ValidationErrors } from '@angular/forms';

export function timeoutValidator(ac: AbstractControl): ValidationErrors | null {
  const value = ac.value;
  if (value == null || value === '' || value === undefined) {
    return null;
  }
  const number = Number.parseFloat(value);
  if (Number.isNaN(number)) {
    return { 'noNumber': { value: value } };
  }
  if (number < 0 || Math.round(number) !== number) {
    return { 'invalidTimeout': { value: value } };
  }
  return null;
}
