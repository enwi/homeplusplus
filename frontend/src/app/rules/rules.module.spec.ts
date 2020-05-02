import { RulesModule } from './rules.module';

describe('RulesModule', () => {
  let rulesModule: RulesModule;

  beforeEach(() => {
    rulesModule = new RulesModule();
  });

  it('should create an instance', () => {
    expect(rulesModule).toBeTruthy();
  });
});
