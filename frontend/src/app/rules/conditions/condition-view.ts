import { RuleCondition } from '../rule';
import { EventEmitter } from '@angular/core';

export interface ConditionView {
  // Called when conditionview is first initialized
  initialize(c: RuleCondition): void;
  // Called when condition changes (same type) after view was initialized
  updateCondition(c: RuleCondition): void;
}
