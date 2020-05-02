import {Component, Inject, OnDestroy, OnInit} from '@angular/core';
import {FormBuilder, FormControl, FormGroup, Validators} from '@angular/forms';
import {MatDialog} from '@angular/material/dialog';
import {ActivatedRoute, Router} from '@angular/router';
import {Observable, Subscription} from 'rxjs';

import {colorToHex, hexToColor} from '../../color-convert.module';
import {DeleteDialog} from '../../delete-dialog/delete-dialog.component';
import {ConditionTypeService} from '../conditions/condition-type.service';
import {RuleConstantCondition} from '../conditions/rule-conditions';
import {Rule, RuleCondition} from '../rule';
import {RULE_ICONS} from '../rule-icons';
import {RuleService} from '../rule.service';

@Component({
  selector: 'app-rule-details',
  templateUrl: './rule-details.component.html',
  styleUrls: ['./rule-details.component.css']
})
export class RuleDetailsComponent implements OnInit, OnDestroy {
  rule: Rule;
  private subscription: Subscription;
  enabled: FormControl;
  edit: FormGroup;
  editEnabled = false;

  // Diese Regel wirklich löschen? Kann nicht rückgängig gemacht werden.
  private dialogText =
      $localize`Really delete this rule? This cannot be undone.`;

  constructor(
      private ruleService: RuleService, private route: ActivatedRoute,
      private router: Router, private dialog: MatDialog,
      private fb: FormBuilder,
      @Inject(RULE_ICONS) public icons: [string, string][]) {}

  ngOnInit() {
    this.getRule();
  }

  ngOnDestroy() {
    if (this.subscription) {
      this.subscription.unsubscribe();
    }
  }

  getRule() {
    const id = +this.route.snapshot.paramMap.get('id');
    this.subscription =
        this.ruleService.getRule(id).subscribe((r: Rule) => this.updateRule(r));
  }

  updateRule(r: Rule) {
    this.rule = r;
    if (!this.enabled) {
      this.enabled = this.fb.control(this.rule.enabled);
      this.enabled.valueChanges.subscribe(
          value => this.onEnabledChanged(value));
    } else {
      // Do not emit events, as it would cause a loop of change messages
      this.enabled.setValue(this.rule.enabled, {emitEvent: false});
    }
  }

  onEnabledChanged(value: any) {
    this.rule.enabled = value as boolean;
    this.ruleService.addRule(this.rule);
  }

  editRule() {
    this.editEnabled = true;
    this.edit = this.fb.group({
      name: [this.rule.name, Validators.required],
      color: [colorToHex(this.rule.color)],
      icon: [this.rule.icon]
    });
  }
  deleteRule() {
    const dialogRef =
        this.dialog.open(DeleteDialog, {data: {content: this.dialogText}});
    dialogRef.afterClosed().subscribe(result => {
      if (result) {
        this.ruleService.deleteRule(this.rule.id);
        this.router.navigate(['/rules']);
      }
    });
  }
  cancelEdit() {
    this.editEnabled = false;
  }
  finishEdit() {
    this.editEnabled = false;
    this.rule.name = this.edit.value.name;
    this.rule.effect.name = this.rule.name + '_Effect';
    this.rule.color = hexToColor(this.edit.value.color);
    this.rule.effect.color = this.rule.color;
    this.rule.icon = this.edit.value.icon;
    this.rule.effect.icon = this.rule.icon;
    this.ruleService.addRule(this.rule);
  }

  sendRule() {
    this.ruleService.addRule(this.rule);
  }

  editCondition(c: RuleCondition) {
    this.rule.condition = c;
    this.sendRule();
  }
  deleteCondition() {
    this.rule.condition = <RuleConstantCondition>{id: 0, type: 0, state: false};
    this.sendRule();
  }
}
