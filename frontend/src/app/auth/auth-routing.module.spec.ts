import { AuthRoutingModule } from './auth-routing.module';

describe('AuthRoutingModule', () => {
  let authRoutingModule: AuthRoutingModule;

  beforeEach(() => {
    authRoutingModule = new AuthRoutingModule();
  });

  it('should create an instance', () => {
    expect(authRoutingModule).toBeTruthy();
  });
});
