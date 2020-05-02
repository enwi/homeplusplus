import {Component, Inject, OnDestroy, OnInit, Type} from '@angular/core';
import {FormBuilder, FormGroup, Validators} from '@angular/forms';
import {MatDialog} from '@angular/material/dialog';
import {ActivatedRoute, Router} from '@angular/router';
import {Observable, Subscription} from 'rxjs';

import {colorToHex, hexToColor} from '../../color-convert.module';
import {DeleteDialog} from '../../delete-dialog/delete-dialog.component';
import {Action, SubAction} from '../action';
import {ACTION_ICONS} from '../action-icons';
import {ActionService} from '../action.service';

@Component({
  selector: 'app-action-details',
  templateUrl: './action-details.component.html',
  styleUrls: ['./action-details.component.css']
})
export class ActionDetailsComponent implements OnInit, OnDestroy {
  action: Action;
  private subscription: Subscription;
  edit: FormGroup;
  editEnabled = false;

  private dialogText =
      $localize`Really delete this action? This cannot be undone.`;

  constructor(
      private actionService: ActionService, private route: ActivatedRoute,
      private router: Router, private fb: FormBuilder,
      private dialog: MatDialog,
      @Inject(ACTION_ICONS) public icons: [string, string][]) {}

  ngOnInit() {
    this.getAction();
  }

  ngOnDestroy() {
    if (this.subscription) {
      this.subscription.unsubscribe();
    }
  }

  getAction() {
    const id = +this.route.snapshot.paramMap.get('id');
    this.subscription =
        this.actionService.getAction(+id).subscribe((a: Action) => {
          this.action = a;
        });
  }

  editAction() {
    this.editEnabled = true;
    this.edit = this.fb.group({
      name: [this.action.name, Validators.required],
      color: [colorToHex(this.action.color)],
      icon: [this.action.icon]
    });
  }
  deleteAction() {
    const dialogRef =
        this.dialog.open(DeleteDialog, {data: {content: this.dialogText}});
    dialogRef.afterClosed().subscribe(result => {
      if (result) {
        this.actionService.deleteAction(this.action.id);
        this.router.navigate(['/actions']);
      }
    });
  }
  cancelEdit() {
    this.editEnabled = false;
  }

  finishEdit() {
    if (this.edit.valid) {
      this.editEnabled = false;
      this.action.name = this.edit.value.name;
      this.action.color = hexToColor(this.edit.value.color);
      this.action.icon = this.edit.value.icon;
      this.actionService.addAction(this.action);
    }
  }

  sendAction() {
    this.actionService.addAction(this.action);
  }
}
