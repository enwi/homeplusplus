import { ActionsModule } from './actions.module';

describe('ActionsModule', () => {
  let actionsModule: ActionsModule;

  beforeEach(() => {
    actionsModule = new ActionsModule();
  });

  it('should create an instance', () => {
    expect(actionsModule).toBeTruthy();
  });
});
