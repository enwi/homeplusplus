import {CommonModule} from '@angular/common';
import {NgModule} from '@angular/core';
import {MatSidenavModule} from '@angular/material/sidenav';

import {FilterListComponent} from './filter-list/filter-list.component';

@NgModule({
  imports: [CommonModule, MatSidenavModule],
  declarations: [FilterListComponent],
  exports: [FilterListComponent]
})
export class FilterListModule {
}
