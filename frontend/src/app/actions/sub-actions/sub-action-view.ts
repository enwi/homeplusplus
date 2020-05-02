import { SubAction } from '../action';
import { EventEmitter } from '@angular/core';

export interface SubActionView {
  initialize(a: SubAction, editAction: EventEmitter<void>, deleteAction: EventEmitter<void>): void;
}
