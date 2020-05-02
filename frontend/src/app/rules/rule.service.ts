import { Injectable } from '@angular/core';
import { Subject, Observable } from 'rxjs';
import { WebsocketChannelService } from '../websocket/websocket-channel.service';
import { Rule } from './rule';
import { map, filter, scan } from 'rxjs/operators';

@Injectable({
  providedIn: 'root'
})
export class RuleService {

  private subject: Subject<any>;

  constructor(private channel: WebsocketChannelService) { }

  private messageToRule(message: any): Rule | null | undefined {
    if (message && 'rule' in message) {
        return Object.assign(new Rule(), message.rule);
    }
    return undefined;
  }

  private initSubject(): void {
    if (!this.subject || this.subject.closed) {
      this.subject = this.channel.getChannel('rules');
    }
  }

  getRules(): Observable<Rule> {
    this.initSubject();
    this.subject.next({ command: 'GET_RULES' });
    return this.subject.pipe(map((message: any) => this.messageToRule(message)),
      filter(rule => rule != null));
  }
  getRule(id: number): Observable<Rule> {
    this.initSubject();
    this.subject.next({ command: 'GET_RULE', ruleId: id });
    return this.subject.pipe(map((message: any) => this.messageToRule(message)),
      filter(rule => rule != null));
  }
  getRulesAsArray(): Observable<Rule[]> {
    // Transform to array, replace repeated rules
    return this.getRules().pipe(scan((list: Rule[], rule: Rule) => {
      const index = list.findIndex(r => r.id === rule.id);
      if (index !== -1) {
        list[index] = rule;
      } else {
        list.push(rule);
      }
      return list;
    }, []));
  }
  deleteRule(id: number): void {
    this.initSubject();
    this.subject.next({ command: 'DELETE_RULE', ruleId: id });
  }
  addRule(rule: Rule): void {
    this.initSubject();
    this.subject.next({ command: 'ADD_RULE', ruleJSON: rule });
  }
}
