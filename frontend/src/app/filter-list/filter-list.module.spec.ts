import { FilterListModule } from './filter-list.module';

describe('FilterListModule', () => {
  let filterListModule: FilterListModule;

  beforeEach(() => {
    filterListModule = new FilterListModule();
  });

  it('should create an instance', () => {
    expect(filterListModule).toBeTruthy();
  });
});
