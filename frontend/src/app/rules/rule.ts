import { Action } from '../actions/action';

export class RuleCondition {
  id: number;
  type: number;
}

export class Rule {
  id: number;
  name: string;
  icon: string;
  color: number;
  condition: RuleCondition;
  effect: Action;
  enabled: boolean;
}
