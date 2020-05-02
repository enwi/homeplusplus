import {Component, ComponentFactoryResolver, ComponentRef, Input, OnChanges, OnDestroy, OnInit, SimpleChanges, Type, ViewChild, ViewContainerRef} from '@angular/core';

@Component({
  selector: 'app-dashboard-card',
  template: `<ng-template #target></ng-template>`,
  styles: [':host {width: 100%; height: 100%}']
})
export class DashboardCardComponent implements OnInit, OnChanges, OnDestroy {
  @Input() component: Type<any>;
  @Input() variables?: {[name: string]: any};
  @ViewChild('target', {read: ViewContainerRef, static: true}) target;
  componentRef: ComponentRef<any>;

  constructor(private componentFactoryResolver: ComponentFactoryResolver) {}

  ngOnInit() {
    this.initView(this.component);
  }

  ngOnChanges(changes: SimpleChanges) {
    const change = changes.component;
    if (change && !change.isFirstChange()) {
      if (change.previousValue !== change.currentValue) {
        this.initView(change.currentValue);
      }
    }
  }

  ngOnDestroy() {
    if (this.componentRef) {
      this.componentRef.destroy();
    }
  }

  private initView(component: Type<any>) {
    if (!component) {
      throw new Error('DashboardCardComponent needs component type');
    }
    const componentFactory =
        this.componentFactoryResolver.resolveComponentFactory(component);
    // this.target.clear();
    this.componentRef = this.target.createComponent(componentFactory);
    if (this.variables) {
      for (const key in this.variables) {
        if (this.variables.hasOwnProperty(key)) {
          this.componentRef.instance[key] = this.variables[key];
        }
      }
    }
  }
}
