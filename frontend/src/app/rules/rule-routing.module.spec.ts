import { RuleRoutingModule } from './rule-routing.module';

describe('RuleRoutingModule', () => {
  let ruleRoutingModule: RuleRoutingModule;

  beforeEach(() => {
    ruleRoutingModule = new RuleRoutingModule();
  });

  it('should create an instance', () => {
    expect(ruleRoutingModule).toBeTruthy();
  });
});
