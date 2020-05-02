import { Directive, Input, Type, Output, EventEmitter, ViewContainerRef,
  ComponentFactoryResolver, OnInit, OnChanges, SimpleChanges, ComponentRef } from '@angular/core';
import { RuleCondition } from '../rule';
import { ConditionView } from './condition-view';
import { ConditionTypeService } from './condition-type.service';
@Directive({
  selector: 'app-condition'
})
export class ConditionDirective implements OnInit, OnChanges {

  @Input() condition: RuleCondition;
  private type: Type<ConditionView>;
  private componentRef: ComponentRef<ConditionView>;

  constructor(private viewContainerRef: ViewContainerRef,
    private componentFactoryResolver: ComponentFactoryResolver,
    private typeService: ConditionTypeService) { }

  ngOnInit() {
    this.type = this.typeService.getViewType(this.condition.type);
    this.initializeView();
  }

  ngOnChanges(changes: SimpleChanges) {
    if (this.componentRef) {
      if ('condition' in changes) {
        const change = changes.condition;
        if (change.previousValue.type !== change.currentValue.type) {
          this.type = this.typeService.getViewType(this.condition.type);
          this.initializeView();
        } else {
          this.componentRef.instance.updateCondition(change.currentValue);
        }
      }
    }
  }

  initializeView() {
    this.viewContainerRef.clear();
    if (this.type && this.condition) {
      const componentFactory = this.componentFactoryResolver.resolveComponentFactory(this.type);
      this.componentRef = this.viewContainerRef.createComponent(componentFactory);
      this.componentRef.instance.initialize(this.condition);
    }
  }
}
