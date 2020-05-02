import { ActionRoutingModule } from './action-routing.module';

describe('ActionRoutingModule', () => {
  let actionRoutingModule: ActionRoutingModule;

  beforeEach(() => {
    actionRoutingModule = new ActionRoutingModule();
  });

  it('should create an instance', () => {
    expect(actionRoutingModule).toBeTruthy();
  });
});
