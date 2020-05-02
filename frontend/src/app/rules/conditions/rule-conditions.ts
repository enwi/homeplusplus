import { RuleCondition } from '../rule';

export class RuleConstantCondition extends RuleCondition {
  state: boolean;
}

export class RuleCompareCondition extends RuleCondition {
  left?: RuleCondition;
  right?: RuleCondition;
  compare: number;
}

export class RuleTimeCondition extends RuleCondition {
  time1: number;
  time2: number;
  compare: number;
  timeType: number;
}

export class RuleDeviceCondition extends RuleCondition {
  value1: number;
  value2: number;
  compare: number;
  deviceId: number;
  property: string;
}
