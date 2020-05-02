import { SubAction } from '../action';
import { Type } from '@angular/core';

export interface TypeForm {
  isValid(): boolean;
  setSubAction(subAction: SubAction): void;
  getSubAction(): SubAction;
}

export class TypeFormInfo {
  displayName: string;
  typeId: number;
  type: Type<TypeForm>;
}
